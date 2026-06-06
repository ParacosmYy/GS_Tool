#include "e33004/m33004.h"
QVector<double> m33004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
