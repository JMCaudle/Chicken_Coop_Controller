#ifndef DATA_POINT
#define DATA_POINT

#include <functional>
#include "Utility.h"

class DataPoint
{
public:
  DataPoint();

  virtual void processValue();
  void setCallback(std::function<void(String)> cbk);
  void clearCallback();

protected:
  std::function<void(String)> onChanged;
};

class BoolData : public DataPoint
{
public:
  // BoolData();
  bool getValue();
  void setValue(bool val);
  void processValue();

private:
  bool _value;
};

class IntData : public DataPoint
{
public:
  // IntData();
  int getValue();
  void setValue(int val);
  void processValue();

private:
  int _value;
};

class FloatData : public DataPoint
{
public:
  // FloatData();
  float getValue();
  void setValue(float val);
  void processValue();

private:
  float _value;
};

class DoubleData : public DataPoint
{
public:
  DoubleData(bool fractionalMinute = false);
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
  // StringData();
  String getValue();
  void setValue(String val);
  void processValue();

private:
  String _value;
};

#endif
