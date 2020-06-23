const char *xml=R"EOS(<?xml version="1.0" encoding="UTF-8"?>
<KvalobsData producer="kvqabase" created="2020-03-16 06:02:34">
  <station val="18700">
    <typeid val="-506">
      <obstime val="2020-03-16 06:00:00">
        <tbtime val="2020-03-16 06:02:27.372995">
          <sensor val="0">
            <level val="0">
              <kvdata paramid="109">
                <original>0</original>
                <corrected>0</corrected>
                <controlinfo>1010000000000000</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
            </level>
          </sensor>
        </tbtime>
      </obstime>
    </typeid>
    <typeid val="316">
      <obstime val="2020-03-16 06:00:00">
        <tbtime val="2020-03-16 06:02:00.559497">
          <sensor val="0">
            <level val="0">
              <kvdata paramid="10">
                <original>2</original>
                <corrected>2</corrected>
                <controlinfo>0100000000000000</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
              <kvdata paramid="14">
                <original>2</original>
                <corrected>2</corrected>
                <controlinfo>0110000000100010</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
              <kvdata paramid="15">
                <original>7</original>
                <corrected>7</corrected>
                <controlinfo>0110000000100010</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
              <kvdata paramid="23">
                <original>5</original>
                <corrected>5</corrected>
                <controlinfo>0110000000000010</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
              <kvdata paramid="24">
                <original>7</original>
                <corrected>7</corrected>
                <controlinfo>0110000000000010</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
              <kvdata paramid="41">
                <original>2</original>
                <corrected>2</corrected>
                <controlinfo>0110000000100010</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
              <kvdata paramid="42">
                <original>2</original>
                <corrected>2</corrected>
                <controlinfo>0110000000000010</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
              <kvdata paramid="43">
                <original>2</original>
                <corrected>2</corrected>
                <controlinfo>0110000000000010</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
              <kvdata paramid="55">
                <original>1000</original>
                <corrected>1000</corrected>
                <controlinfo>0110000000100000</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
              <kvdata paramid="112">
                <original>-1</original>
                <corrected>-1</corrected>
                <controlinfo>0110000000000010</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
              <kvdata paramid="129">
                <original>1</original>
                <corrected>1</corrected>
                <controlinfo>0110000000000000</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
              <kvdata paramid="273">
                <original>70000</original>
                <corrected>70000</corrected>
                <controlinfo>0110000000100010</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
            </level>
          </sensor>
          <kvtextdata paramid="1000">
            <original>SC</original>
          </kvtextdata>
          <kvtextdata paramid="1022">
            <original>20200316060000</original>
          </kvtextdata>
        </tbtime>
        <tbtime val="2020-03-16 06:02:33.806730">
          <sensor val="0">
            <level val="0">
              <kvdata paramid="0">
                <original>-32767</original>
                <corrected>-32767</corrected>
                <controlinfo>0010003000000000</controlinfo>
                <useinfo>7899900000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
            </level>
          </sensor>
        </tbtime>
      </obstime>
    </typeid>
    <typeid val="501">
      <obstime val="2020-03-16 06:00:00">
        <tbtime val="2020-03-16 06:01:58.186947">
          <sensor val="0">
            <level val="0">
              <kvdata paramid="112">
                <original>0.839999974</original>
                <corrected>-1</corrected>
                <controlinfo>0000004000000400</controlinfo>
                <useinfo>7033900000000001</useinfo>
                <cfailed>QC1-0-autosnow_501,QC1-0-autosnow_501</cfailed>
              </kvdata>
            </level>
          </sensor>
        </tbtime>
      </obstime>
    </typeid>
    <typeid val="506">
      <obstime val="2020-03-16 06:00:00">
        <tbtime val="2020-03-16 06:01:53.741158">
          <sensor val="0">
            <level val="0">
              <kvdata paramid="81">
                <original>8.60000038</original>
                <corrected>8.60000038</corrected>
                <controlinfo>0000000000100000</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
              <kvdata paramid="211">
                <original>2.70000005</original>
                <corrected>2.70000005</corrected>
                <controlinfo>0010000000100000</controlinfo>
                <useinfo>7000000000000000</useinfo>
                <cfailed></cfailed>
              </kvdata>
            </level>
          </sensor>
        </tbtime>
      </obstime>
    </typeid>
  </station>
</KvalobsData>
)EOS";



const char *conf=R"EOS(
wmo_default{
  #default values
  copyto="/var/lib/kvbufrd/bufr2norcom"
  copy="true"
#  copy="false"
  owner="AUTG"
  #owner="KVAL"
  list="99"
  loglevel=9
}

wmo_1492 {
   code=(0)
   name="OSLO-BLINDERN"
   height=94
   height_wind=26
   height_precip=1
   height_temperature=2
   height_pressure=96.6
   stationid=18700
   typepriority=(316,"*501")
   delay=("SS:05")
   precipitation=("RR_1")
   owner="HYBR"
   latitude=59.9423
   longitude=10.72
}
)EOS";