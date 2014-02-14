import unittest
import mock

from evePathfinder.stateinterface import AutopilotPathfinderInterface, DEFAULT_AVOIDANCE

AVOID_LIST = "autopilot_avoidance2"
AVOID_ENABLE = "pfAvoidSystems"
ROUTE_TYPE = "pfRouteType"
AVOID_PODKILL_ENABLE = "pfAvoidPodKill"

class AutopilotStateInterfaceTestCase(unittest.TestCase):
    def setUp(self):
        self.settings = {
            AVOID_ENABLE: 0,
            AVOID_LIST: None,
            ROUTE_TYPE: "safe",
            AVOID_PODKILL_ENABLE: 0,
        }
        self.uiSettings = mock.Mock()
        self.uiSettings.Get.side_effect = self.settings.get
        self.uiSettings.Set.side_effect = self.settings.__setitem__
        self.updatePodKills = mock.Mock()
        self.mapSvc = mock.Mock()
        self.stateInterface = AutopilotPathfinderInterface(
            self.mapSvc,
            self.updatePodKills,
            self.uiSettings,
        )

    def testGetAvoidanceListWithSettingsDisabledReturnsNothing(self):
        avoided = self.stateInterface.GetAvoidanceList()
        self.assertEqual(avoided, [])

    def testGetAvoidanceListWithAvoidAndPodKillEnabledReturnsCombinedList(self):
        avoidList = [5, 3]
        self.settings[AVOID_LIST] = avoidList
        self.settings[AVOID_ENABLE] = 1
        self.settings[AVOID_PODKILL_ENABLE] = 1

        self.mapSvc.ExpandItems.return_value = avoidList
        self.updatePodKills.return_value = [2]
        avoided = self.stateInterface.GetAvoidanceList()
        self.updatePodKills.assert_called_once_with([])
        self.assertEqual(avoided, [2, 3, 5])
