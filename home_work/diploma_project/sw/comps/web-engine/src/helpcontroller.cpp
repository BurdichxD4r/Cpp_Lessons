#include "helpcontroller.h"

HelpController::HelpController(QObject *parent)
    : HttpRequestHandler{parent}
{

}
//-------------------------------------------------------------------
void HelpController::service(HttpRequest &request, HttpResponse &response) {

    response.setHeader("Content-Type", "text/html; charset=UTF-8");
    response.write("<!DOCTYPE html>");
    response.write("<html><head>");
    response.write("<meta charset=\"UTF-8\">");
    response.write("<title>Помощь</title>");
    response.write("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />");
    response.write("<link rel=\"stylesheet\" href=\"/style.css\">");
    response.write("</head");

    response.write("<body>");

    response.write("<h1>Помощь если забыли пароль</h1>");
    response.write("<div id=\"wrapper\">");
    response.write("<h2>Если пароль был утерен, следует обратится к администаратору или зайти в систему как наблюдатель (test:hello)</h2>");
    response.write("<p> <a href=\"/login\">На страницу аутентификации</a></p>");
    response.write("</form>");
    response.write("</div>");

    response.write("</body></html>",true);
}
//----------------------------------------------------------------------------
