#include "f35205/m35205.h"
QVector<double> m35205::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
