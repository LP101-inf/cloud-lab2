#include <iostream>
#include <ctime>
#include <string>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <memory>
#include <array>
#include <map>
#include <sstream>
#include <iomanip>
#include <cmath>

#define PORT 8080

// Uruchamianie curl 

std::string exec(const char* cmd) {
    std::array<char, 256> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "";

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();

    return result;
}

// Otrzymywanie czasu 

std::string getCurrentTime() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
    return oss.str();
}

// Parsowanie danych pogodowych

std::string extractBlock(const std::string& data) {
    size_t s = data.find("\"current_weather\":{");
    if (s == std::string::npos) return "";
    s += 19;

    size_t e = data.find('}', s);
    if (e == std::string::npos) return "";

    return data.substr(s, e - s);
}

double getValue(const std::string& text, const std::string& key) {
    std::string k = "\"" + key + "\":";
    size_t p = text.find(k);
    if (p == std::string::npos) return NAN;

    p += k.size();
    while (p < text.size() && (text[p] == ' ')) p++;

    size_t e = p;
    while (e < text.size() &&
          (isdigit(text[e]) || text[e] == '.' || text[e] == '-')) {
        e++;
    }

    if (e == p) return NAN;

    return std::stod(text.substr(p, e - p));
}

std::string desc(double code) {
    int c = (int)code;
    if (c == 0) return "Czyste niebo";
    if (c <= 3) return "Lekko pochmurno";
    if (c <= 48) return "Mgła";
    if (c <= 67) return "Deszcz";
    if (c <= 77) return "Śnieg";
    return "Burza";
}

//Html

std::string form() {

    return R"(<!DOCTYPE html>
<html>
<head>
<meta charset='UTF-8'>
<title>Pogoda</title>

<style>
body{
    font-family:'Segoe UI',Arial,sans-serif;
    background:#0f172a;
    color:white;
    text-align:center;
    padding-top:60px;
}

.box{
    background:#1e293b;
    padding:30px 40px;
    border-radius:16px;
    display:inline-block;
    box-shadow:0 4px 20px rgba(0,0,0,0.5);
}

h2{
    margin-top:0;
    color:#38bdf8;
}

select,button{
    padding:12px 18px;
    margin:10px;
    border-radius:8px;
    border:none;
    font-size:16px;
}

button{
    background:#38bdf8;
    color:#0f172a;
    font-weight:bold;
    cursor:pointer;
}

button:hover{
    background:#0284c7;
}
</style>

<script>
const cities = {
    "Polska": ["Lublin", "Warszawa"],
    "Niemcy": ["Berlin", "Monachium"]
};

function updateCities() {

    const country = document.getElementById("country").value;
    const citySelect = document.getElementById("city");

    citySelect.innerHTML = "";

    if (!cities[country]) {
        const option = document.createElement("option");
        option.text = "-- wybierz miasto --";
        option.value = "";
        citySelect.add(option);
        return;
    }

    cities[country].forEach(function(city) {
        const option = document.createElement("option");
        option.text = city;
        option.value = city;
        citySelect.add(option);
    });
}

window.onload = function() {
    updateCities();
};
</script>

</head>

<body>

<div class='box'>

<h2>Pogoda</h2>

<form action='/weather' method='get'>

<select name='country' id='country' onchange='updateCities()' required>
    <option value='Polska'>Polska</option>
    <option value='Niemcy'>Niemcy</option>
</select>

<select name='city' id='city' required>
</select>

<br>

<button type='submit'>Sprawdź</button>

</form>

</div>

</body>
</html>)";
}

std::string weatherPage(std::string country, std::string city) {

    std::map<std::pair<std::string,std::string>, std::pair<double,double>> coords = {
        {{"Polska","Lublin"}, {51.25, 22.57}},
        {{"Polska","Warszawa"}, {52.23, 21.01}},
        {{"Niemcy","Berlin"}, {52.52, 13.41}},
        {{"Niemcy","Monachium"}, {48.14, 11.58}}
    };

    auto key = std::make_pair(country, city);
    if (coords.find(key) == coords.end())
        return form();

    double lat = coords[key].first;
    double lon = coords[key].second;

    char cmd[300];
    snprintf(cmd, sizeof(cmd),
        "curl -s --max-time 3 'https://api.open-meteo.com/v1/forecast?latitude=%.2f&longitude=%.2f&current_weather=true'",
        lat, lon);

    std::string data = exec(cmd);
    std::string cw = extractBlock(data);

    double temp = getValue(cw, "temperature");
    double wind = getValue(cw, "windspeed");
    double code = getValue(cw, "weathercode");

    std::ostringstream out;

    out << R"(
<!DOCTYPE html>
<html>
<head>
<meta charset='UTF-8'>
<style>
body{font-family:Arial;background:#020617;color:white;text-align:center;padding-top:60px;}
.box{background:#1e293b;padding:30px;border-radius:12px;display:inline-block;}
.big{font-size:40px;color:#38bdf8;}
</style>
</head>
<body>
<div class='box'>
<h2>Pogoda )" << city << R"(</h2>
<p class='big'>)" << temp << R"( °C</p>
<p>Wiatr: )" << wind << R"( km/h</p>
<p>)" << desc(code) << R"(</p>
<br>
<a href='/'>Powrót</a>
</div>
</body>
</html>)";

    return out.str();
}

//Main

int main() {

    std::cout << "Autor: Łukasz Pisula" << std::endl;
    std::cout << "Start: " << getCurrentTime() << std::endl;
    std::cout << "Port: " << PORT << std::endl;

    int server = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server, (sockaddr*)&addr, sizeof(addr));
    listen(server, 10);

    while (true) {

        int client = accept(server, nullptr, nullptr);

        char buffer[2048] = {0};
        read(client, buffer, 2048);

        std::string req(buffer);
        std::string response;

        if (req.find("GET /weather") != std::string::npos) {

            std::string country, city;

            size_t c = req.find("country=");
            if (c != std::string::npos) {
                size_t s = c + 8;
                size_t e = req.find('&', s);
                if (e == std::string::npos) e = req.find(' ', s);
                country = req.substr(s, e - s);
            }

            size_t m = req.find("city=");
            if (m != std::string::npos) {
                size_t s = m + 5;
                size_t e = req.find(' ', s);
                city = req.substr(s, e - s);
            }

            response = weatherPage(country, city);

        } else {
            response = form();
        }

        std::string http =
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + response;

        send(client, http.c_str(), http.size(), 0);
        close(client);
    }
}