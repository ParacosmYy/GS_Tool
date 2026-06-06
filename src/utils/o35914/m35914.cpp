#include "o35914/m35914.h"
QVector<double> m35914::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
