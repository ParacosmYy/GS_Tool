#include "l14011/m14011.h"
QVector<double> m14011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
