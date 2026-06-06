#include "b15121/m15121.h"
QVector<double> m15121::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
