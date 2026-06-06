#include "t35559/m35559.h"
QVector<double> m35559::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
