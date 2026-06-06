#include "s9938/m9938.h"
QVector<double> m9938::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
