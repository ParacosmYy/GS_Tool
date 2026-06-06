#include "l18831/m18831.h"
QVector<double> m18831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
