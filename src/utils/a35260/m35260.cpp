#include "a35260/m35260.h"
QVector<double> m35260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
