#ifndef DATA_POINT
#define DATA_POINT

#include <functional>
#include <Preferences.h>
#include "Utility.h"

class DataPoint
{
public:
  DataPoint();

  virtual void makeDataPersist(String key);

  virtual void processValue();
  void setCallback(std::function<void(String, bool)> cbk);
  void clearCallback();

protected:
  std::function<void(String, bool)> onChanged;
  String _key;
};

class BoolData : public DataPoint
{
public:
  // BoolData();
  void makeDataPersist(String key, bool preset = false);
  bool getValue();
  void setValue(bool val);
  void processValue();

private:
  bool _value;
};

class IntData : public DataPoint
{
public:
  IntData(bool minutes = false, int lowValue = -1, int highValue = -2);
  void makeDataPersist(String key, int preset = 0);
  int getValue();
  void setValue(int val);
  void processValue();

private:
  int _value;
  int _lowValue;
  int _highValue;
  bool _minutes;
};

class FloatData : public DataPoint
{
public:
  FloatData(float lowValue = -1, float highValue = -2);
  void makeDataPersist(String key, float preset = 0.0);
  float getValue();
  void setValue(float val);
  void processValue();

private:
  float _value;
  float _lowValue;
  float _highValue;
};

class DoubleData : public DataPoint
{
public:
  DoubleData(bool fractionalMinute = false);
  void makeDataPersist(String key, double preset = 0.0);
  double getValue();
  void setValue(double val);
  void processValue();

private:
  double _value;
  bool _fractionalMinute;
};

class StringData : public DataPoint
{
public:
  StringData(String ideal = "");
  void makeDataPersist(String key, String preset = "");
  String getValue();
  void setValue(String val);
  void processValue();

private:
  String _value;
  String _idealValue;
};

#endif
