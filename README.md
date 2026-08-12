## Wordell: A Wordle Clone for Windows 1.0

<img src="images/logo.jpg" align="left" width="250" alt="Wordell Logo">

Wordell is a Wordle clone for Windows 1.0 written in C. It features
2,315 possible unique games.

I don't know if there are any Windows 1.0 enthusiasts out there, but if
you are, this could be your jam. Mouse is available but isn't used.
It's basically a wrapper around the DOS 3.3 version, and the changes
are solely visual. It's also the exact same game code from the
[Apple II version](https://github.com/mpower-codeworks/WORDELL-Wordle-Clone-for-Apple-II).

### Why man, _why_?

I guess I just wanted to have an excuse to do some actual work in Windows 1.0, and since the
DOS version went swimmingly it seemed like I should have a go. Windows 1.0 is pretty cool.
I found that Windows 1.0 is  really best thought of as a hybrid system, because half the 
time you'll want to jump back into DOS to get things done.

I did most of the development on a modern PC; doing it all in Windows 1.0 would be torture.
However, a fair amount was indeed done on 1.0 because it was necessary. I already had the
game itself from the DOS version, and I used the "Hello" example from the Windows
SDK 1.0.3 as the template (from Disk 6 of the SDK). Edlin was used very often, and any
time is a good time for Edlin. Right?

## How Wordell for Windows 1.0 Works

The Windows 1.0 version of Wordell was built by using the original HELLO SDK sample
as the framework, and then the existing DOS Wordell code runs inside it. HELLO provides the
startup, message loop, window procedure, resources, et al. Thanks to whomever made those
samples - they're great. 

- Built with Microsoft C 4.0 and the Windows 1.03 SDK
- DOS-style screen output was replaced with Windows GDI
- Keyboard input is handled through Windows messages
- Screen updates are handled through WM_PAINT and InvalidateRect
- Windows resources provide the application name, icon, About dialog

## Screenshots

<table>
    <tr>
        <td align="left" width="50%" valign="middle">
            <img src="images/windows1title.jpg" width="100%" alt="Windows 1 Title"><br>
        </td>
        <td align="left" width="50%" valign="middle">
            <img src="images/windows1help.jpg" width="100%" alt="Windows 1 Help"><br>
        </td>
    </tr>
    <tr>
        <td align="left" width="50%" valign="middle">
            <img src="images/windows1board.jpg" width="100%" alt="Windows 1 "><br>
        </td>
        <td align="left" width="50%" valign="middle">
            <img src="images/windows1stats.jpg" width="100%" alt="Windows 1 "><br>
        </td>
    </tr>
</table>


