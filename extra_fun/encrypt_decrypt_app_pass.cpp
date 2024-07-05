#include <iostream>
#include <string>

using namespace std;

// Encrypt function
string caesarEncrypt(const string& text, int shift) {
    string result = "";

    // Traverse the text
    for (int i = 0; i < text.length(); i++) {
        if (isupper(text[i])) {
            result += char(int(text[i] + shift - 65) % 26 + 65);
        }
        else if (islower(text[i])) {
            result += char(int(text[i] + shift - 97) % 26 + 97);
        }
        else {
            result += text[i]; // Non-alphabetical characters are unchanged
        }
    }

    return result;
}

// Decrypt function
string caesarDecrypt(const string& text, int shift) {
    return caesarEncrypt(text, 26 - shift);
}

int main() 
{
    string password = "pass123";
    int shift = 12; // Shift key

    // Encrypt the password
    string encrypted = caesarEncrypt(password, shift);
    cout << "Encrypted: " << encrypted << endl;

    // Decrypt the password
    string decrypted = caesarDecrypt(encrypted, shift);
    cout << "Decrypted: " << decrypted << endl;

    return 0;
}
