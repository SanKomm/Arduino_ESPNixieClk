import requests

r = requests.get("https://gist.githubusercontent.com/alwynallan/24d96091655391107939/raw/9eb0a986bb5a78586523f5ec0d6897b37c61bff1/tz_data.json")
data = r.json()
file = open("timezones.h","w")

file.write("const char *timezones = R\"(\n")
file.write("<select name=\"timeZone\" id=\"zone\" onchange=\"document.getElementById('key_custom2').value = this.value\">\n")
for region in data["regions"]:
   for zone in region["zones"]:
       file.write("<option value=""%s"">%s/%s</option>\n" % (
          zone["tz"],
          region["name"],
          zone["name"]))
file.write("</select>\n")
file.write("<script>\n")
file.write("document.getElementById('zone').value = GMT0>Africa/Abidjan;\n")
file.write("document.querySelector(\"[for='key_custom2']\").hidden = true;\n")
file.write("document.getElementById('key_custom2').hidden = true;\n")
file.write("</script>)\";")
file.close()