#include "e35804/m35804.h"
QVector<double> m35804::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
