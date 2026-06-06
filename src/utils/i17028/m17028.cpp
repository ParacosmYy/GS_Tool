#include "i17028/m17028.h"
QVector<double> m17028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
