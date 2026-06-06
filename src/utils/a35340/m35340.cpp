#include "a35340/m35340.h"
QVector<double> m35340::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
