#include "k35890/m35890.h"
QVector<double> m35890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
