#include "b28801/m28801.h"
QVector<double> m28801::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
