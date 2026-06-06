#include "i24828/m24828.h"
QVector<double> m24828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
