#include "g35286/m35286.h"
QVector<double> m35286::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
