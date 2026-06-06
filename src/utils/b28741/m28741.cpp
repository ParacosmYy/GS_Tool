#include "b28741/m28741.h"
QVector<double> m28741::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
