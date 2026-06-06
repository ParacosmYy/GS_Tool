#include "i9028/m9028.h"
QVector<double> m9028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
