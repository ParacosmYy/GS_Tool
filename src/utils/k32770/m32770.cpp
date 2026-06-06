#include "k32770/m32770.h"
QVector<double> m32770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
