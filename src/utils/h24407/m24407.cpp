#include "h24407/m24407.h"
QVector<double> m24407::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
