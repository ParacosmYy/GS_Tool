#include "k35670/m35670.h"
QVector<double> m35670::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
