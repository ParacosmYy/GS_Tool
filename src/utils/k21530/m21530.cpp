#include "k21530/m21530.h"
QVector<double> m21530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
