#include "s35938/m35938.h"
QVector<double> m35938::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
