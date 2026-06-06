#include "k21010/m21010.h"
QVector<double> m21010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
