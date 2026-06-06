#include "k21910/m21910.h"
QVector<double> m21910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
