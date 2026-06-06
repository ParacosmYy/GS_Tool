#include "d35103/m35103.h"
QVector<double> m35103::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
