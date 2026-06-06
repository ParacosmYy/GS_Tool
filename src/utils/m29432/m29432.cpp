#include "m29432/m29432.h"
QVector<double> m29432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
