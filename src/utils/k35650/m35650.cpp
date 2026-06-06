#include "k35650/m35650.h"
QVector<double> m35650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
