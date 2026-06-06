#include "e35224/m35224.h"
QVector<double> m35224::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
