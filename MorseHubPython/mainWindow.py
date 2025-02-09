import wx
import wx.adv
from morsePlayer import MorseGenerator, AudioSignal
import datetime
import user
ID_RUFZ = wx.NewIdRef()
ID_RUNNER = wx.NewIdRef()
ID_RX = wx.NewIdRef()
ID_TX = wx.NewIdRef()
ID_SITE = wx.NewIdRef()
ID_GITHUB = wx.NewIdRef()
ID_DONATE = wx.NewIdRef()
ID_UPDATE = wx.NewIdRef()
ID_BETA = wx.NewIdRef()
ID_JOINSERVER = wx.NewIdRef()
ID_SCOREBOARD = wx.NewIdRef()
ID_MAILME = wx.NewIdRef()
ID_OPT = wx.NewIdRef()
class WelcomeDialog(wx.Dialog):

    def __init__(self, parent=None):
        super().__init__(parent, wx.ID_ANY, "MorseHub setup")
        self.generator = MorseGenerator(AudioSignal.Sine)
        sizer = wx.BoxSizer(wx.VERTICAL)

        sizer.Add(wx.StaticText(self, label="Welcome to MorseHub"), 0, wx.ALL | wx.CENTER, 10)
        sizer.Add(wx.StaticText(self, label="Type your username"), 0, wx.ALL, 5)
        self.nameTextCTRL = wx.TextCtrl(self, value=user.user.name)
        sizer.Add(self.nameTextCTRL, 0, wx.ALL | wx.EXPAND, 5)

        sizer.Add(wx.StaticText(self, label="Type your callsign, leave it blank if you don't have one"), 0, wx.ALL, 5)
        self.callsignTextCTRL = wx.TextCtrl(self, value=user.user.call)
        sizer.Add(self.callsignTextCTRL, 0, wx.ALL | wx.EXPAND, 5)

        sizer.Add(wx.StaticText(self, label="Date of Birth"), 0, wx.ALL, 5)
        self.datePickerCTRL = wx.adv.DatePickerCtrl(self, dt=wx.DateTime.FromDMY(user.user.dateOfBirth.day, user.user.dateOfBirth.month-1, user.user.dateOfBirth.year))
        sizer.Add(self.datePickerCTRL, 0, wx.ALL | wx.EXPAND, 5)

        sizer.Add(wx.StaticText(self, label="Speed"), 0, wx.ALL, 5)
        self.speedCTRL = wx.SpinCtrl(self, value=str(user.user.settings.speed), min=10, max=1000)
        sizer.Add(self.speedCTRL, 0, wx.ALL | wx.EXPAND, 5)

        sizer.Add(wx.StaticText(self, label="Pitch"), 0, wx.ALL, 5)
        self.pitchCTRL = wx.SpinCtrl(self, value=str(user.user.settings.frequency), min=400, max=1001)
        sizer.Add(self.pitchCTRL, 0, wx.ALL | wx.EXPAND, 5)

        sizer.Add(wx.StaticText(self, label="Signal"), 0, wx.ALL, 5)
        self.signalChoice = wx.Choice(self, choices=user.user.settings.signalsString)
        self.signalChoice.SetStringSelection(user.user.settings.audioSignalString)
        sizer.Add(self.signalChoice, 0, wx.ALL | wx.EXPAND, 5)

        testBTN = wx.Button(self, label="Test")
        testBTN.Bind(wx.EVT_BUTTON, self.OnTest)
        sizer.Add(testBTN, 0, wx.ALL | wx.CENTER, 5)

        ok = wx.Button(self, wx.ID_OK)
        sizer.Add(ok, 0, wx.ALL | wx.CENTER, 5)
        self.SetSizerAndFit(sizer)
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.signalChoice.Bind(wx.EVT_CHOICE, self.OnSignalChanged)
        ok.Bind(wx.EVT_BUTTON, self.OnOK)
        
    def OnTest(self, event):
        try:
            self.generator.speed = int(self.speedCTRL.GetValue())
            self.generator.freq = int(self.pitchCTRL.GetValue())
            self.generator.audioSignal = user.settings.audioSignal
            self.generator.transmit("hello, world")
        except Exception as e:
            wx.MessageBox(f"Error: {e}", "Error", wx.OK | wx.ICON_ERROR)
    
    def OnClose(self, evt):
        self.Destroy()
    
    def OnSignalChanged(self, evt):
        user.user.settings.audioSignalString = self.signalChoice.GetStringSelection()
        user.user.settings.audioSignal = user.user.settings.signalsString.index(user.user.settings.audioSignalString)
    def OnOK(self, evt):
        #retrieve the values from our controls
        user.user.name = self.nameTextCTRL.GetValue()
        user.user.call = self.callsignTextCTRL.GetValue()
        curdate: wx.DateTime = self.datePickerCTRL.GetValue()
        user.user.dateOfBirth = datetime.date(curdate.GetYear(), curdate.GetMonth()+1, curdate.GetDay())
        user.user.settings.speed = int(self.speedCTRL.GetValue())
        user.user.settings.frequency = int(self.pitchCTRL.GetValue())
        user.user.settings.audioSignalString = self.signalChoice.GetStringSelection()
        user.user.settings.audioSignal = user.user.settings.signalsString.index(user.user.settings.audioSignalString)
        if user.user.name=="":
            wx.MessageBox(message="Name cannot be empty", caption="Missing name", style=wx.OK | wx.ICON_ERROR)
            return
        self.EndModal(wx.OK)

class MainWindow(wx.Frame):
    def __init__(self, parent=None):
        super().__init__(parent, wx.ID_ANY, "MorseHub")
        self.id_start = wx.NewIdRef()
        panel = wx.Panel(self)
        sizer = wx.BoxSizer(wx.VERTICAL)
        startBTN = wx.Button(panel, self.id_start, "&Start")
        volumelabel = wx.StaticText(panel, label = "Volume")
        volumeSlider = wx.Slider(panel, wx.ID_ANY, 50)
        exitBTN = wx.Button(panel, wx.ID_EXIT, "E&xit")
        sizer.Add(startBTN, 0, wx.ALL | wx.CENTER, 10)
        sizer.Add(volumelabel, 0, wx.ALL, 5)
        sizer.Add(volumeSlider, 0, wx.EXPAND, 5)
        sizer.Add(exitBTN, 0, wx.ALL | wx.CENTER, 10)
        panel.SetSizer(sizer)
        sizer.Fit(self)
        #self.Bind(wx.EVT_SHOW, self.OnAppear)
        self._menu_set()
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        startBTN.Bind(wx.EVT_BUTTON, self.OnStart)
        exitBTN.Bind(wx.EVT_BUTTON, self.OnExit)
        self.SetAcceleratorTable(wx.AcceleratorTable([(wx.ACCEL_NORMAL, wx.WXK_F5, self.id_start)]))
    def _menu_set(self):
        menuBar = wx.MenuBar()
        fm = wx.Menu()
        om = wx.Menu()
        hm = wx.Menu()
        mm = wx.Menu() #mode menu
        mm.Append(ID_RUFZ, "RufzXP", kind = wx.ITEM_CHECK)
        mm.Append(ID_RUNNER, "Morse Runner", kind = wx.ITEM_CHECK)
        mm.Append(ID_RX, "Reception", kind = wx.ITEM_CHECK)
        mm.Append(ID_TX, "Transmission (experimental)", kind = wx.ITEM_CHECK)
        mm.Append(ID_JOINSERVER, "HST competitions", kind = wx.ITEM_CHECK)
        om.AppendSubMenu(mm, "&Mode")
        om.Append(ID_OPT, "MorseHub &prefferences")
        fm.Append(wx.ID_NEW, "&Import a scoreboard file")
        fm.Append(wx.ID_DELETE, "&Empty your scoreboard")
        fm.Append(wx.ID_EDIT, "&Edit your profile")
        fm.Append(ID_SCOREBOARD, "Show &scoreboard")
        fm.Append(wx.ID_EXIT, "E&xit MorseHub")
        hm.Append(wx.ID_ABOUT, "&About MorseHub")
        hm.Append(ID_UPDATE, "Check for MorseHub updates")
        hm.Append(ID_BETA, "Download beta versions of MorseHub")
        hm.Append(ID_DONATE, "Suport the project")
        hm.Append(ID_MAILME, "Mail me")
        hm.Append(ID_SITE, "Open my website")
        hm.Append(ID_GITHUB, "Open MorseHub github page to open a pull request, an issue or clone the repository")
        menuBar.Append(fm, "&File")
        menuBar.Append(om, "&Options")
        menuBar.Append(hm, "&Help")
        self.SetMenuBar(menuBar)
        #bindings later
    def OnClose(self, evt):
        evt.Skip()
    def OnExit(self, evt):
        self.Close()
    def OnStart(self, evt):
        menubar = self.GetMenuBar()

        item_rufz = menubar.FindItemById(ID_RUFZ)
        item_runner = menubar.FindItemById(ID_RUNNER)
        item_rx = menubar.FindItemById(ID_RX)

        if item_rufz and item_rufz.IsChecked():
            pass
        elif item_runner and item_runner.IsChecked():
            pass
        elif item_rx and item_rx.IsChecked():
            pass
        else:
            wx.MessageBox("Please select a mode from the menu bar before starting", "No mode selected", wx.OK | wx.ICON_ERROR)
