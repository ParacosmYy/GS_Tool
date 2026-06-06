#include "t35179/m35179.h"
QVector<double> m35179::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
