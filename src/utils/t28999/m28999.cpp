#include "t28999/m28999.h"
QVector<double> m28999::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
