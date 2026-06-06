#include "a9600/m9600.h"
QVector<double> m9600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
