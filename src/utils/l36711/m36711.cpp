#include "l36711/m36711.h"
QVector<double> m36711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
