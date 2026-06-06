#include "k29450/m29450.h"
QVector<double> m29450::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
