#include "l15251/m15251.h"
QVector<double> m15251::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
