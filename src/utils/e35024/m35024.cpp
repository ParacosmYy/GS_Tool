#include "e35024/m35024.h"
QVector<double> m35024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
