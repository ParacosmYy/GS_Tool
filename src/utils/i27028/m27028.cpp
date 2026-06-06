#include "i27028/m27028.h"
QVector<double> m27028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
