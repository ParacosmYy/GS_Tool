#include "a24420/m24420.h"
QVector<double> m24420::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
