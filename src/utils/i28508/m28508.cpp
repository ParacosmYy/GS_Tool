#include "i28508/m28508.h"
QVector<double> m28508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
