#include "e36704/m36704.h"
QVector<double> m36704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
