#include "m29172/m29172.h"
QVector<double> m29172::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
