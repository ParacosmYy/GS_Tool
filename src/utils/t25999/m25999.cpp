#include "t25999/m25999.h"
QVector<double> m25999::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
