#include "i24188/m24188.h"
QVector<double> m24188::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
