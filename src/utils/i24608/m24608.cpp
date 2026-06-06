#include "i24608/m24608.h"
QVector<double> m24608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
