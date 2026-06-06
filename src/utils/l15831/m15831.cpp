#include "l15831/m15831.h"
QVector<double> m15831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
