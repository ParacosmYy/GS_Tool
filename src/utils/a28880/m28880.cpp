#include "a28880/m28880.h"
QVector<double> m28880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
