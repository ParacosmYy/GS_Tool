#include "k18250/m18250.h"
QVector<double> m18250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
