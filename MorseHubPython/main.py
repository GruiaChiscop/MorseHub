import wx
import user
import mainWindow
#app initialisation logic comes here
class MorseHubApp(wx.App):
    def OnInit(self):
        #return super().OnInit(self)
        dialog = mainWindow.WelcomeDialog()
        mainwindow = mainWindow.MainWindow()
        u = user.deserializeUser()
        if not u:
            if dialog.ShowModal() == wx.OK:
                self.SetTopWindow(mainwindow)
                dialog.Destroy()
                user.serializeUser(user.user)
                mainwindow.Show()
            else:
                dialog.Destroy()
                return False
            return True
        else:
            user.user = u
            mainwindow.Show()
            dialog.Destroy()
            return True
app = MorseHubApp()
app.MainLoop()