#include "g21126/m21126.h"
QVector<double> m21126::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
