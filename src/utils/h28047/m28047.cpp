#include "h28047/m28047.h"
QVector<double> m28047::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
