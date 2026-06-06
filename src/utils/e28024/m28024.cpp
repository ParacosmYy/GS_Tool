#include "e28024/m28024.h"
QVector<double> m28024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
