#include "o32914/m32914.h"
QVector<double> m32914::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
