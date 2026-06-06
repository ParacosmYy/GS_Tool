#include "g24106/m24106.h"
QVector<double> m24106::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
